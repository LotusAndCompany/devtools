# Localization

The React UI supports English and Japanese. UI text is kept beside the feature
or in the shared localization helper. Use the existing
`text(locale, english, japanese)` helper and pass the active locale through
component props where needed.

When changing a visible string:

1. Provide both English and Japanese wording.
2. Check both locales in the affected screen or dialog.
3. Update tests and user documentation when behavior or instructions change.

There is no Qt Linguist catalog or compiled `.qm` file in the application.
