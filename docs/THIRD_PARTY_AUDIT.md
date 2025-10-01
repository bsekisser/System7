# Third-Party Audit Program

We welcome independent verification of our System 7.1 reimplementation's clean-room status.

## Scope

Third-party auditors will verify:

1. **No implementation usage**: No code from `supermario/` or other Apple leaks
2. **ROM-based derivation**: All implementations traceable to binary analysis of legitimate ROMs
3. **Public API only**: All Apple names/constants from published Inside Macintosh documentation
4. **Reproducible build**: ISO builds bit-for-bit from source

## Auditor Eligibility

We seek reviewers with:

- **Security researchers**: Experience in software provenance, license compliance, or forensics
- **Legal experts**: IP attorneys specializing in software copyright
- **OSS maintainers**: Leaders of established open-source projects
- **Academic researchers**: Computer science professors or PhD students studying software engineering

## Public Audit (No NDA)

All materials in this repository are public. Auditors can:

1. Clone repository and run analysis tools (see `ReviewerPlaybook.md`)
2. Review similarity reports (`prove/SIMILARITY_REPORT.md`)
3. Verify build reproducibility (`prove/Dockerfile`)
4. Publish findings openly

**No confidential access required.**

## Private Audit (With NDA)

For deeper review, we can share under NDA:

- **Full disassembly notes**: Ghidra/radare2 project files with ROM annotations
- **AI prompt logs**: Complete LLM conversation history showing no implementation ingestion
- **Original ROM images**: For independent verification of derivation claims

**Deliverable:** Signed audit summary (template below)

### NDA Terms

- **Purpose**: Verify clean-room status only (not competitive analysis)
- **Duration**: 90 days
- **Confidentiality**: Disassembly notes and prompts remain private; summary is public
- **No liability**: Auditor provides opinion only, not legal advice

**To request NDA audit:**
Open GitHub issue with tag `audit-request-nda` including:
- Your name/organization
- Relevant credentials (LinkedIn, prior audit work, etc.)
- Proposed timeline (we suggest 2-4 weeks)

## Audit Summary Template

```markdown
# System 7.1 Reimplementation - Independent Audit Summary

**Auditor**: [Name], [Organization/Affiliation]
**Audit period**: [Start date] to [End date]
**Materials reviewed**: [Public only / Public + private under NDA]

## Methodology

[Brief description of analysis approach: tools used, files reviewed, etc.]

## Findings

### Similarity Analysis
- PMD CPD (120+ tokens): [N] cross-repo matches
- Winnowing SimHash: [X]% overall similarity
- Assessment: [PASS / REVIEW NEEDED / FAIL]

### implementation Markers
- Apple copyright: [N] instances
- SCCS/version control tags: [N] instances
- Assessment: [PASS / REVIEW NEEDED / FAIL]

### Derivation Evidence
- ROM address citations: [N] files
- Inside Macintosh citations: [N] references
- Assessment: [SUFFICIENT / INSUFFICIENT]

### Build Reproducibility
- ISO hash matches: [YES / NO]
- Assessment: [PASS / FAIL]

## Conclusion

[Overall assessment: CLEAN-ROOM / NEEDS REMEDIATION / CONTAMINATED]

[Detailed justification, any concerns, recommendations]

## Auditor Statement

I, [Name], certify that I conducted this audit independently and in good faith. To the best of my knowledge, the System 7.1 Portable project [IS / IS NOT / UNCLEAR] a clean-room reimplementation based on binary reverse engineering.

**Signature**: [Digital signature or PGP-signed message]
**Date**: [Date]
**PGP fingerprint**: [If applicable]
```

## Privacy & Data Handling

- **Public audit**: All findings can be published freely
- **NDA audit**: Only the summary above is public; disassembly notes remain confidential
- **ROM images**: We will not distribute ROM files; auditors must source legitimately

## Timeline

- **Public audit**: Self-paced (review `ReviewerPlaybook.md`)
- **NDA audit**: Typical 2-4 weeks from NDA signing to summary publication

## Compensation

This is a volunteer program. We do not currently offer payment, but will:

- Acknowledge auditors in `PROVENANCE.md`
- Cite audit summary in public statements
- Provide letter of recommendation upon request (for students/researchers)

## Contact

**GitHub issue**: https://github.com/Kelsidavis/System7/issues (tag: `audit-request` or `audit-request-nda`)
**Email**: [TBD - maintainer email if desired]

---
*Audit program version: 1.0*
*Last updated: 2025-03-30*
