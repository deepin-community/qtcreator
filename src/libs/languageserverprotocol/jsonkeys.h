// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

namespace LanguageServerProtocol {

constexpr char actionsKey[] = "actions";
constexpr char activeParameterKey[] = "activeParameter";
constexpr char activeParameterSupportKey[] = "activeParameterSupport";
constexpr char activeSignatureKey[] = "activeSignature";
constexpr char addedKey[] = "added";
constexpr char additionalTextEditsKey[] = "additionalTextEdits";
constexpr char alphaKey[] = "alpha";
constexpr char appliedKey[] = "applied";
constexpr char applyEditKey[] = "applyEdit";
constexpr char argumentsKey[] = "arguments";
constexpr char blueKey[] = "blue";
constexpr char callHierarchyKey[] = "callHierarchy";
constexpr char callHierarchyProviderKey[] = "callHierarchyProvider";
constexpr char cancellableKey[] = "cancellable";
constexpr char capabilitiesKey[] = "capabilities";
constexpr char chKey[] = "ch";
constexpr char changeKey[] = "change";
constexpr char changeNotificationsKey[] = "changeNotifications";
constexpr char changesKey[] = "changes";
constexpr char characterKey[] = "character";
constexpr char childrenKey[] = "children";
constexpr char clientInfoKey[] = "clientInfo";
constexpr char codeActionKey[] = "codeAction";
constexpr char codeActionKindKey[] = "codeActionKind";
constexpr char codeActionKindsKey[] = "codeActionKinds";
constexpr char codeActionLiteralSupportKey[] = "codeActionLiteralSupport";
constexpr char codeActionProviderKey[] = "codeActionProvider";
constexpr char codeKey[] = "code";
constexpr char codeLensKey[] = "codeLens";
constexpr char codeLensProviderKey[] = "codeLensProvider";
constexpr char colorInfoKey[] = "colorInfo";
constexpr char colorKey[] = "color";
constexpr char colorProviderKey[] = "colorProvider";
constexpr char commandKey[] = "command";
constexpr char commandsKey[] = "commands";
constexpr char commitCharacterSupportKey[] = "commitCharacterSupport";
constexpr char commitCharactersKey[] = "commitCharacters";
constexpr char completionItemKey[] = "completionItem";
constexpr char completionItemKindKey[] = "completionItemKind";
constexpr char completionKey[] = "completion";
constexpr char completionProviderKey[] = "completionProvider";
constexpr char configurationKey[] = "configuration";
constexpr char containerNameKey[] = "containerName";
constexpr char contentChangesKey[] = "contentChanges";
constexpr char contentFormatKey[] = "contentFormat";
constexpr char contentKey[] = "value";
constexpr char contentsKey[] = "contents";
constexpr char contextKey[] = "context";
constexpr char contextSupportKey[] = "contextSupport";
constexpr char dataKey[] = "data";
constexpr char definitionKey[] = "definition";
constexpr char definitionProviderKey[] = "definitionProvider";
constexpr char deleteCountKey[] = "deleteCount";
constexpr char deltaKey[] = "delta";
constexpr char deprecatedKey[] = "deprecated";
constexpr char detailKey[] = "detail";
constexpr char diagnosticsKey[] = "diagnostics";
constexpr char didChangeConfigurationKey[] = "didChangeConfiguration";
constexpr char didChangeWatchedFilesKey[] = "didChangeWatchedFiles";
constexpr char didSaveKey[] = "didSave";
constexpr char documentChangesKey[] = "documentChanges";
constexpr char documentFormattingProviderKey[] = "documentFormattingProvider";
constexpr char documentHighlightKey[] = "documentHighlight";
constexpr char documentHighlightProviderKey[] = "documentHighlightProvider";
constexpr char documentLinkKey[] = "documentLink";
constexpr char documentLinkProviderKey[] = "documentLinkProvider";
constexpr char documentRangeFormattingProviderKey[] = "documentRangeFormattingProvider";
constexpr char documentSelectorKey[] = "documentSelector";
constexpr char documentSymbolKey[] = "documentSymbol";
constexpr char documentSymbolProviderKey[] = "documentSymbolProvider";
constexpr char documentationFormatKey[] = "documentationFormat";
constexpr char documentationKey[] = "documentation";
constexpr char dynamicRegistrationKey[] = "dynamicRegistration";
constexpr char editKey[] = "edit";
constexpr char editsKey[] = "edits";
constexpr char endKey[] = "end";
constexpr char errorKey[] = "error";
constexpr char eventKey[] = "event";
constexpr char executeCommandKey[] = "executeCommand";
constexpr char executeCommandProviderKey[] = "executeCommandProvider";
constexpr char experimentalKey[] = "experimental";
constexpr char filterTextKey[] = "filterText";
constexpr char firstTriggerCharacterKey[] = "firstTriggerCharacter";
constexpr char formatsKey[] = "formats";
constexpr char formattingKey[] = "formatting";
constexpr char fromKey[] = "from";
constexpr char fromRangesKey[] = "fromRanges";
constexpr char fullKey[] = "full";
constexpr char greenKey[] = "green";
constexpr char hierarchicalDocumentSymbolSupportKey[] = "hierarchicalDocumentSymbolSupport";
constexpr char hoverKey[] = "hover";
constexpr char hoverProviderKey[] = "hoverProvider";
constexpr char idKey[] = "id";
constexpr char ignoreIfExistsKey[] = "ignoreIfExists";
constexpr char ignoreIfNotExistsKey[] = "ignoreIfNotExists";
constexpr char implementationKey[] = "implementation";
constexpr char implementationProviderKey[] = "implementationProvider";
constexpr char includeDeclarationKey[] = "includeDeclaration";
constexpr char includeTextKey[] = "includeText";
constexpr char initializationOptionsKey[] = "initializationOptions";
constexpr char insertFinalNewlineKey[] = "insertFinalNewline";
constexpr char insertSpaceKey[] = "insertSpace";
constexpr char insertTextFormatKey[] = "insertTextFormat";
constexpr char insertTextKey[] = "insertText";
constexpr char isIncompleteKey[] = "isIncomplete";
constexpr char itemKey[] = "item";
constexpr char itemsKey[] = "items";
constexpr char jsonRpcVersionKey[] = "jsonrpc";
constexpr char kindKey[] = "kind";
constexpr char labelKey[] = "label";
constexpr char languageIdKey[] = "languageId";
constexpr char languageKey[] = "language";
constexpr char legendKey[] = "legend";
constexpr char limitKey[] = "limit";
constexpr char lineKey[] = "line";
constexpr char linesKey[] = "lines";
constexpr char locationKey[] = "location";
constexpr char messageKey[] = "message";
constexpr char methodKey[] = "method";
constexpr char moreTriggerCharacterKey[] = "moreTriggerCharacter";
constexpr char multiLineTokenSupportKey[] = "multiLineTokenSupport";
constexpr char nameKey[] = "name";
constexpr char newNameKey[] = "newName";
constexpr char newTextKey[] = "newText";
constexpr char newUriKey[] = "newUri";
constexpr char oldUriKey[] = "oldUri";
constexpr char onTypeFormattingKey[] = "onTypeFormatting";
constexpr char onlyKey[] = "only";
constexpr char openCloseKey[] = "openClose";
constexpr char optionsKey[] = "options";
constexpr char overlappingTokenSupportKey[] = "overlappingTokenSupport";
constexpr char overwriteKey[] = "overwrite";
constexpr char parametersKey[] = "parameters";
constexpr char paramsKey[] = "params";
constexpr char patternKey[] = "pattern";
constexpr char percentageKey[] = "percentage";
constexpr char placeHolderKey[] = "placeHolder";
constexpr char positionKey[] = "position";
constexpr char prepareProviderKey[] = "prepareProvider";
constexpr char prepareSupportKey[] = "prepareSupport";
constexpr char previousResultIdKey[] = "previousResultId";
constexpr char processIdKey[] = "processId";
constexpr char queryKey[] = "query";
constexpr char rangeFormattingKey[] = "rangeFormatting";
constexpr char rangeKey[] = "range";
constexpr char rangeLengthKey[] = "rangeLength";
constexpr char reasonKey[] = "reason";
constexpr char recursiveKey[] = "recursive";
constexpr char redKey[] = "red";
constexpr char referencesKey[] = "references";
constexpr char referencesProviderKey[] = "referencesProvider";
constexpr char refreshSupportKey[] = "refreshSupport";
constexpr char registerOptionsKey[] = "registerOptions";
constexpr char registrationsKey[] = "registrations";
constexpr char removedKey[] = "removed";
constexpr char renameKey[] = "rename";
constexpr char renameProviderKey[] = "renameProvider";
constexpr char requestsKey[] = "requests";
constexpr char resolveProviderKey[] = "resolveProvider";
constexpr char resourceOperationsKey[] = "resourceOperations";
constexpr char resultIdKey[] = "resultId";
constexpr char resultKey[] = "result";
constexpr char retryKey[] = "retry";
constexpr char rootPathKey[] = "rootPath";
constexpr char rootUriKey[] = "rootUri";
constexpr char saveKey[] = "save";
constexpr char schemeKey[] = "scheme";
constexpr char scopeUriKey[] = "scopeUri";
constexpr char sectionKey[] = "section";
constexpr char selectionRangeKey[] = "selectionRange";
constexpr char semanticTokensKey[] = "semanticTokens";
constexpr char semanticTokensProviderKey[] = "semanticTokensProvider";
constexpr char serverInfoKey[] = "serverInfo";
constexpr char settingsKey[] = "settings";
constexpr char severityKey[] = "severity";
constexpr char signatureHelpKey[] = "signatureHelp";
constexpr char signatureHelpProviderKey[] = "signatureHelpProvider";
constexpr char signatureInformationKey[] = "signatureInformation";
constexpr char signaturesKey[] = "signatures";
constexpr char snippetSupportKey[] = "snippetSupport";
constexpr char sortTextKey[] = "sortText";
constexpr char sourceKey[] = "source";
constexpr char startKey[] = "start";
constexpr char supportedKey[] = "supported";
constexpr char symbolKey[] = "symbol";
constexpr char symbolKindKey[] = "symbolKind";
constexpr char syncKindKey[] = "syncKind";
constexpr char synchronizationKey[] = "synchronization";
constexpr char tabSizeKey[] = "tabSize";
constexpr char tagsKey[] = "tags";
constexpr char targetKey[] = "target";
constexpr char textDocumentKey[] = "textDocument";
constexpr char textDocumentSyncKey[] = "textDocumentSync";
constexpr char textEditKey[] = "textEdit";
constexpr char textKey[] = "text";
constexpr char titleKey[] = "title";
constexpr char toKey[] = "to";
constexpr char tokenKey[] = "token";
constexpr char tokenModifiersKey[] = "tokenModifiers";
constexpr char tokenTypesKey[] = "tokenTypes";
constexpr char traceKey[] = "trace";
constexpr char triggerCharacterKey[] = "triggerCharacter";
constexpr char triggerCharactersKey[] = "triggerCharacters";
constexpr char triggerKindKey[] = "triggerKind";
constexpr char trimFinalNewlinesKey[] = "trimFinalNewlines";
constexpr char trimTrailingWhitespaceKey[] = "trimTrailingWhitespace";
constexpr char typeDefinitionKey[] = "typeDefinition";
constexpr char typeDefinitionProviderKey[] = "typeDefinitionProvider";
constexpr char typeKey[] = "type";
constexpr char unregistrationsKey[] = "unregistrations";
constexpr char uriKey[] = "uri";
constexpr char valueKey[] = "value";
constexpr char valueSetKey[] = "valueSet";
constexpr char versionKey[] = "version";
constexpr char willSaveKey[] = "willSave";
constexpr char willSaveWaitUntilKey[] = "willSaveWaitUntil";
constexpr char windowKey[] = "window";
constexpr char workDoneProgressKey[] = "workDoneProgress";
constexpr char workspaceEditKey[] = "workspaceEdit";
constexpr char workspaceFoldersKey[] = "workspaceFolders";
constexpr char workspaceKey[] = "workspace";
constexpr char workspaceSymbolProviderKey[] = "workspaceSymbolProvider";

} // namespace LanguageServerProtocol
