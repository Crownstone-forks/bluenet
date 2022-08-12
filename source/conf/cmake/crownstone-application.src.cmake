######################################################################################################
# If a source file does not strictly depend on the platform to build for, it should be defined here.
######################################################################################################

message(STATUS "crownstone application source files appended to FOLDER_SOURCE")

LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_Behaviour.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_BehaviourHandler.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_BehaviourConflictResolution.cpp")

LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_BehaviourStore.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_ExtendedSwitchBehaviour.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_SwitchBehaviour.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_TwilightBehaviour.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/behaviour/cs_TwilightHandler.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/common/cs_Component.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/common/cs_Types.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/events/cs_Event.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/events/cs_EventDispatcher.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/events/cs_EventListener.cpp")

LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/presence/cs_PresenceHandler.cpp")

list(APPEND FOLDER_SOURCE "${SOURCE_DIR}/util/cs_BitmaskVarSize.cpp")
list(APPEND FOLDER_SOURCE "${SOURCE_DIR}/util/cs_Hash.cpp")

LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/cfg/cs_Boards.c")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/util/cs_WireFormat.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/time/cs_TimeOfDay.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/time/cs_SystemTime.cpp")
LIST(APPEND FOLDER_SOURCE "${SOURCE_DIR}/uart/cs_UartHandler.cpp")