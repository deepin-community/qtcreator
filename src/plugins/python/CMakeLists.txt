add_qtc_plugin(Python
  DEPENDS QmlJS
  PLUGIN_DEPENDS Core LanguageClient ProjectExplorer TextEditor
  SOURCES
    python.qrc
    pythonconstants.h
    pythoneditor.cpp pythoneditor.h
    pythonformattoken.h
    pythonhighlighter.cpp pythonhighlighter.h
    pythonindenter.cpp pythonindenter.h
    pythonlanguageclient.cpp pythonlanguageclient.h
    pythonplugin.cpp pythonplugin.h
    pythonproject.cpp pythonproject.h
    pythonrunconfiguration.cpp pythonrunconfiguration.h
    pythonscanner.cpp pythonscanner.h
    pythonsettings.cpp pythonsettings.h
    pythonutils.cpp pythonutils.h
)
