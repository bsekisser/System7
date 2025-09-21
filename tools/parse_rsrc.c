/*
 * parse_rsrc.c - Parse Mac resource fork files
 * Extracts font resources from .rsrc files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Resource fork structures */
#pragma pack(push, 1)

typedef struct {
    uint32_t data_offset;    /* Offset to resource data */
    uint32_t map_offset;     /* Offset to resource map */
    uint32_t data_length;    /* Length of resource data */
    uint32_t map_length;     /* Length of resource map */
} ResourceHeader;

typedef struct {
    uint8_t  reserved1[16];  /* Reserved for system use */
    uint32_t reserved2;
    uint16_t reserved3;
    uint16_t attributes;     /* Resource file attributes */
    uint16_t type_list_offset; /* Offset from beginning of map to type list */
    uint16_t name_list_offset; /* Offset from beginning of map to name list */
} ResourceMap;

typedef struct {
    uint16_t num_types_minus_1; /* Number of types minus 1 */
} TypeListHeader;

typedef struct {
    uint32_t type_code;      /* Four-character type code */
    uint16_t num_resources_minus_1; /* Number of resources of this type minus 1 */
    uint16_t ref_list_offset; /* Offset from type list to reference list */
} TypeListEntry;

typedef struct {
    uint16_t resource_id;    /* Resource ID */
    uint16_t name_offset;    /* Offset to name */
    uint8_t  attributes;     /* Resource attributes */
    uint8_t  data_offset_hi; /* High byte of data offset */
    uint16_t data_offset_lo; /* Low word of data offset */
    uint32_t reserved;       /* Reserved */
} ResourceRef;

#pragma pack(pop)

/* Byte swapping for big-endian data */
uint16_t swap16(uint16_t val) {
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
}

uint32_t swap32(uint32_t val) {
    return ((val & 0xFF) << 24) |
           ((val & 0xFF00) << 8) |
           ((val & 0xFF0000) >> 8) |
           ((val >> 24) & 0xFF);
}

void print_type(uint32_t type) {
    char str[5];
    str[0] = (type >> 24) & 0xFF;
    str[1] = (type >> 16) & 0xFF;
    str[2] = (type >> 8) & 0xFF;
    str[3] = type & 0xFF;
    str[4] = 0;
    printf("'%s'", str);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <resource_file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        printf("Cannot open file: %s\n", argv[1]);
        return 1;
    }

    /* Read resource header */
    ResourceHeader header;
    fread(&header, sizeof(ResourceHeader), 1, f);

    /* Convert from big-endian */
    header.data_offset = swap32(header.data_offset);
    header.map_offset = swap32(header.map_offset);
    header.data_length = swap32(header.data_length);
    header.map_length = swap32(header.map_length);

    printf("Resource File: %s\n", argv[1]);
    printf("Data offset: 0x%08X\n", header.data_offset);
    printf("Map offset:  0x%08X\n", header.map_offset);
    printf("Data length: 0x%08X\n", header.data_length);
    printf("Map length:  0x%08X\n", header.map_length);
    printf("\n");

    /* Read resource map */
    fseek(f, header.map_offset, SEEK_SET);
    ResourceMap map;
    fread(&map, sizeof(ResourceMap), 1, f);

    map.type_list_offset = swap16(map.type_list_offset);
    map.name_list_offset = swap16(map.name_list_offset);

    /* Read type list */
    fseek(f, header.map_offset + map.type_list_offset, SEEK_SET);
    TypeListHeader type_header;
    fread(&type_header, sizeof(TypeListHeader), 1, f);
    type_header.num_types_minus_1 = swap16(type_header.num_types_minus_1);

    int num_types = type_header.num_types_minus_1 + 1;
    printf("Number of resource types: %d\n\n", num_types);

    /* Read each type */
    for (int i = 0; i < num_types; i++) {
        TypeListEntry type_entry;
        long type_pos = ftell(f);
        fread(&type_entry, sizeof(TypeListEntry), 1, f);

        type_entry.type_code = swap32(type_entry.type_code);
        type_entry.num_resources_minus_1 = swap16(type_entry.num_resources_minus_1);
        type_entry.ref_list_offset = swap16(type_entry.ref_list_offset);

        int num_resources = type_entry.num_resources_minus_1 + 1;

        printf("Type ");
        print_type(type_entry.type_code);
        printf(": %d resources\n", num_resources);

        /* Save position and read resource references */
        long next_type_pos = ftell(f);
        fseek(f, header.map_offset + map.type_list_offset + type_entry.ref_list_offset, SEEK_SET);

        for (int j = 0; j < num_resources; j++) {
            ResourceRef ref;
            fread(&ref, sizeof(ResourceRef), 1, f);

            ref.resource_id = swap16(ref.resource_id);
            ref.name_offset = swap16(ref.name_offset);

            /* Calculate data offset */
            uint32_t data_offset = (ref.data_offset_hi << 16) | swap16(ref.data_offset_lo);

            printf("  ID %d: offset 0x%08X", ref.resource_id, header.data_offset + data_offset);

            /* Read resource name if it has one */
            if (ref.name_offset != 0xFFFF) {
                long cur_pos = ftell(f);
                fseek(f, header.map_offset + map.name_list_offset + ref.name_offset, SEEK_SET);
                uint8_t name_len;
                fread(&name_len, 1, 1, f);
                char name[256];
                fread(name, 1, name_len, f);
                name[name_len] = 0;
                printf(" \"%s\"", name);
                fseek(f, cur_pos, SEEK_SET);
            }

            /* If it's a font resource, save it */
            if (type_entry.type_code == 0x464F4E44 || /* 'FOND' */
                type_entry.type_code == 0x4E464E54 || /* 'NFNT' */
                type_entry.type_code == 0x73666E74) { /* 'sfnt' */

                long cur_pos = ftell(f);

                /* Read resource data length */
                fseek(f, header.data_offset + data_offset, SEEK_SET);
                uint32_t res_length;
                fread(&res_length, sizeof(uint32_t), 1, f);
                res_length = swap32(res_length);

                /* Save to file */
                char filename[256];
                char typestr[5];
                typestr[0] = (type_entry.type_code >> 24) & 0xFF;
                typestr[1] = (type_entry.type_code >> 16) & 0xFF;
                typestr[2] = (type_entry.type_code >> 8) & 0xFF;
                typestr[3] = type_entry.type_code & 0xFF;
                typestr[4] = 0;

                sprintf(filename, "%s_%d.bin", typestr, ref.resource_id);

                uint8_t *data = malloc(res_length);
                fread(data, 1, res_length, f);

                FILE *out = fopen(filename, "wb");
                if (out) {
                    fwrite(data, 1, res_length, out);
                    fclose(out);
                    printf(" -> saved to %s", filename);
                }
                free(data);

                fseek(f, cur_pos, SEEK_SET);
            }

            printf("\n");
        }

        /* Return to next type entry */
        fseek(f, next_type_pos, SEEK_SET);
        printf("\n");
    }

    fclose(f);
    return 0;
}