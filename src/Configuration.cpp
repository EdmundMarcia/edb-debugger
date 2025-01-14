/*
Copyright (C) 2006 - 2023 Evan Teran
						  evan.teran@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Configuration.h"
#include "edb.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDesktopServices>
#include <QDir>
#include <QFont>
#include <QSettings>
#include <QtDebug>

#define STRINGIFY(x) #x
#define TOSTRING(x)  STRINGIFY(x)

namespace {

QDataStream &operator<<(QDataStream &s, const IBreakpoint::TypeId &id) {
	return s << static_cast<int>(id);
}

QDataStream &operator>>(QDataStream &s, IBreakpoint::TypeId &id) {
	int value = 0;
	s >> value;
	id = static_cast<IBreakpoint::TypeId>(value);
	return s;
}

//------------------------------------------------------------------------------
// Name: getDefaultPluginPath
// Desc: return default path for plugins
//------------------------------------------------------------------------------
static QString getDefaultPluginPath() {
#ifdef DEFAULT_PLUGIN_PATH
	const QString default_plugin_path = DEFAULT_PLUGIN_PATH;
#else
	const QString edb_lib_dir    = QCoreApplication::applicationDirPath() + (EDB_IS_64_BIT ? "/../lib64/edb" : "/../lib/edb");
	const QString edb_binary_dir = QCoreApplication::applicationDirPath();
	// If the binary is in its installation directory, then look for plugins in their installation directory
	// Otherwise assume that we are in build directory, so the plugins are in the same directory as the binary
	const QString default_plugin_path = QRegExp(".*/bin/?$").exactMatch(edb_binary_dir) ? edb_lib_dir : edb_binary_dir;
#endif
	return default_plugin_path;
}

}

//------------------------------------------------------------------------------
// Name: Configuration
// Desc: constructor
//------------------------------------------------------------------------------
Configuration::Configuration(QObject *parent)
	: QObject(parent) {
	readSettings();
}

//------------------------------------------------------------------------------
// Name: ~Configuration
// Desc: destructor
//------------------------------------------------------------------------------
Configuration::~Configuration() {
	writeSettings();
}

//------------------------------------------------------------------------------
// Name: sendChangeNotification
// Desc: emits the settingsUpdated signal
//------------------------------------------------------------------------------
void Configuration::sendChangeNotification() {
	Q_EMIT settingsUpdated();
}

//------------------------------------------------------------------------------
// Name: read_settings
// Desc: read in the options from the file
//------------------------------------------------------------------------------
void Configuration::readSettings() {

	qRegisterMetaTypeStreamOperators<IBreakpoint::TypeId>("IBreakpoint::TypeId");

#ifdef Q_OS_WIN32
	const QString default_font = QFont("Courier New", 8).toString();
#elif defined(Q_OS_MACX)
	const QString default_font        = QFont("Courier New", 10).toString();
#else
	const QString default_font = QFont("Monospace", 8).toString();
#endif

	QSettings settings;

	settings.beginGroup("General");
	close_behavior = static_cast<CloseBehavior>(settings.value("close_behavior").value<uint>());
	settings.endGroup();

	settings.beginGroup("Appearance");
	stack_font              = settings.value("appearance.stack.font", default_font).toString();
	data_font               = settings.value("appearance.data.font", default_font).toString();
	registers_font          = settings.value("appearance.registers.font", default_font).toString();
	disassembly_font        = settings.value("appearance.disassembly.font", default_font).toString();
	data_show_address       = settings.value("appearance.data.show_address.enabled", true).toBool();
	data_show_hex           = settings.value("appearance.data.show_hex.enabled", true).toBool();
	data_show_ascii         = settings.value("appearance.data.show_ascii.enabled", true).toBool();
	data_show_comments      = settings.value("appearance.data.show_comments.enabled", true).toBool();
	data_word_width         = settings.value("appearance.data.word_width", 1).value<int>();
	data_row_width          = settings.value("appearance.data.row_width", 16).value<int>();
	show_address_separator  = settings.value("appearance.address_colon.enabled", true).toBool();
	show_jump_arrow         = settings.value("appearance.show_jump_arrow.enabled", true).toBool();
	function_offsets_in_hex = settings.value("appearance.function_offsets_in_hex.enabled", false).toBool();
	theme_name              = settings.value("appearance.theme", "System").toString();

	settings.endGroup();

	settings.beginGroup("Debugging");
	initial_breakpoint      = static_cast<InitialBreakpoint>(settings.value("debugger.initial_breakpoint", MainSymbol).value<uint>());
	warn_on_no_exec_bp      = settings.value("debugger.BP_NX_warn.enabled", true).toBool();
	find_main               = settings.value("debugger.find_main.enabled", true).toBool();
	min_string_length       = settings.value("debugger.string_min", 4).value<uint>();
	tty_enabled             = settings.value("debugger.terminal.enabled", true).toBool();
	tty_command             = settings.value("debugger.terminal.command", "/usr/bin/xterm").toString();
	remove_stale_symbols    = settings.value("debugger.remove_stale_symbols.enabled", true).toBool();
	disableASLR             = settings.value("debugger.disableASLR.enabled", false).toBool();
	disableLazyBinding      = settings.value("debugger.disableLazyBinding.enabled", false).toBool();
	break_on_library_load   = settings.value("debugger.break_on_library_load_event.enabled", false).toBool();
	default_breakpoint_type = settings.value("debugger.default_breakpoint_type", QVariant::fromValue(IBreakpoint::TypeId::Automatic)).value<IBreakpoint::TypeId>();
	settings.endGroup();

	settings.beginGroup("Disassembly");
	syntax                                 = static_cast<Syntax>(settings.value("disassembly.syntax", Intel).value<uint>());
	syntax_highlighting_enabled            = settings.value("disassembly.syntax_highlighting_enabled", true).toBool();
	zeros_are_filling                      = settings.value("disassembly.zeros_are_filling.enabled", true).toBool();
	uppercase_disassembly                  = settings.value("disassembly.uppercase.enabled", false).toBool();
	small_int_as_decimal                   = settings.value("disassembly.small_int_as_decimal.enabled", false).toBool();
	tab_between_mnemonic_and_operands      = settings.value("disassembly.tab_between_mnemonic_and_operands.enabled", false).toBool();
	show_register_badges                   = settings.value("disassembly.show_register_badges.enabled", true).toBool();
	show_local_module_name_in_jump_targets = settings.value("disassembly.show_local_module_name_in_jump_targets.enabled", true).toBool();
	show_symbolic_addresses                = settings.value("disassembly.show_symbolic_addresses.enabled", true).toBool();
	simplify_rip_relative_targets          = settings.value("disassembly.simplify_rip_relative_targets.enabled", true).toBool();
	settings.endGroup();

	settings.beginGroup("Directories");

	QStringList cacheDirectories = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
	QString cacheDirectory       = !cacheDirectories.isEmpty() ? cacheDirectories[0] : QString();

	QString defaultSymbolPath  = QString("%1/%2").arg(cacheDirectory, "symbols");
	QString defaultSessionPath = QString("%1/%2").arg(cacheDirectory, "sessions");

	symbol_path  = settings.value("directory.symbol.path", defaultSymbolPath).toString();
	plugin_path  = settings.value("directory.plugin.path", getDefaultPluginPath()).toString();
	session_path = settings.value("directory.session.path", defaultSessionPath).toString();
	settings.endGroup();

	settings.beginGroup("Exceptions");
	enable_signals_message_box = settings.value("signals.show_message_box.enabled", true).toBool();

	QVariantList temp_ignored_exceptions = settings.value("signals.ignore_list", QVariantList()).toList();

	ignored_exceptions.clear();
	for (QVariant &exception : temp_ignored_exceptions) {
		ignored_exceptions.push_back(exception.toLongLong());
	}

	settings.endGroup();

	settings.beginGroup("Window");
	startup_window_location = static_cast<StartupWindowLocation>(settings.value("window.startup_window_location", SystemDefault).value<uint>());
	settings.endGroup();

	if (startup_window_location < 0 || startup_window_location > 2) {
		startup_window_location = SystemDefault;
	}

	// normalize values
	if (data_word_width != 1 && data_word_width != 2 && data_word_width != 4 && data_word_width != 8) {
		data_word_width = 1;
	}

	if (data_row_width != 1 && data_row_width != 2 && data_row_width != 4 && data_row_width != 8 && data_row_width != 16) {
		data_row_width = 16;
	}

	// Init capstone to some default settings
#if defined(EDB_X86) || defined(EDB_X86_64)
	CapstoneEDB::init(EDB_IS_64_BIT ? CapstoneEDB::Architecture::ARCH_AMD64 : CapstoneEDB::Architecture::ARCH_X86);
#elif defined(EDB_ARM32)
	CapstoneEDB::init(CapstoneEDB::Architecture::ARCH_ARM32_ARM);
#elif defined(EDB_ARM64)
	CapstoneEDB::init(CapstoneEDB::Architecture::ARCH_ARM64);
#else
#error "How to initialize Capstone?"
#endif
	CapstoneEDB::Formatter::FormatOptions options = edb::v1::formatter().options();
	options.capitalization                        = uppercase_disassembly ? CapstoneEDB::Formatter::UpperCase : CapstoneEDB::Formatter::LowerCase;
	options.syntax                                = static_cast<CapstoneEDB::Formatter::Syntax>(syntax);
	options.tabBetweenMnemonicAndOperands         = tab_between_mnemonic_and_operands;
	options.simplifyRIPRelativeTargets            = simplify_rip_relative_targets;
	edb::v1::formatter().setOptions(options);
}

//------------------------------------------------------------------------------
// Name: write_settings
// Desc: writes the options to the file
//------------------------------------------------------------------------------
void Configuration::writeSettings() {

	QSettings settings;

	settings.beginGroup("General");
	settings.setValue("close_behavior", close_behavior);
	settings.endGroup();

	settings.beginGroup("Appearance");
	settings.setValue("appearance.stack.font", stack_font);
	settings.setValue("appearance.data.font", data_font);
	settings.setValue("appearance.registers.font", registers_font);
	settings.setValue("appearance.disassembly.font", disassembly_font);
	settings.setValue("appearance.data.show_address.enabled", data_show_address);
	settings.setValue("appearance.data.show_hex.enabled", data_show_hex);
	settings.setValue("appearance.data.show_ascii.enabled", data_show_ascii);
	settings.setValue("appearance.data.show_comments.enabled", data_show_comments);
	settings.setValue("appearance.data.word_width", data_word_width);
	settings.setValue("appearance.data.row_width", data_row_width);
	settings.setValue("appearance.address_colon.enabled", show_address_separator);
	settings.setValue("appearance.show_jump_arrow.enabled", show_jump_arrow);
	settings.setValue("appearance.function_offsets_in_hex.enabled", function_offsets_in_hex);
	settings.setValue("appearance.theme", theme_name);
	settings.endGroup();

	settings.beginGroup("Debugging");
	settings.setValue("debugger.BP_NX_warn.enabled", warn_on_no_exec_bp);
	settings.setValue("debugger.string_min", min_string_length);
	settings.setValue("debugger.initial_breakpoint", initial_breakpoint);
	settings.setValue("debugger.find_main.enabled", find_main);
	settings.setValue("debugger.terminal.enabled", tty_enabled);
	settings.setValue("debugger.terminal.command", tty_command);
	settings.setValue("debugger.remove_stale_symbols.enabled", remove_stale_symbols);
	settings.setValue("debugger.disableASLR.enabled", disableASLR);
	settings.setValue("debugger.disableLazyBinding.enabled", disableLazyBinding);
	settings.setValue("debugger.break_on_library_load_event.enabled", break_on_library_load);
	settings.setValue("debugger.default_breakpoint_type", QVariant::fromValue(default_breakpoint_type));
	settings.endGroup();

	settings.beginGroup("Disassembly");
	settings.setValue("disassembly.syntax", syntax);
	settings.setValue("disassembly.syntax_highlighting_enabled", syntax_highlighting_enabled);
	settings.setValue("disassembly.zeros_are_filling.enabled", zeros_are_filling);
	settings.setValue("disassembly.uppercase.enabled", uppercase_disassembly);
	settings.setValue("disassembly.small_int_as_decimal.enabled", small_int_as_decimal);
	settings.setValue("disassembly.tab_between_mnemonic_and_operands.enabled", tab_between_mnemonic_and_operands);
	settings.setValue("disassembly.show_local_module_name_in_jump_targets.enabled", show_local_module_name_in_jump_targets);
	settings.setValue("disassembly.show_symbolic_addresses.enabled", show_symbolic_addresses);
	settings.setValue("disassembly.simplify_rip_relative_targets.enabled", simplify_rip_relative_targets);
	settings.setValue("disassembly.show_register_badges.enabled", show_register_badges);
	settings.endGroup();

	settings.beginGroup("Directories");
	settings.setValue("directory.symbol.path", symbol_path);
	if (plugin_path != getDefaultPluginPath()) {
		settings.setValue("directory.plugin.path", plugin_path);
	} else {
		settings.remove("directory.plugin.path");
	}
	settings.setValue("directory.session.path", session_path);
	settings.endGroup();

	settings.beginGroup("Exceptions");
	settings.setValue("signals.show_message_box.enabled", enable_signals_message_box);
	QVariantList temp_ignored_exceptions;

	Q_FOREACH (qlonglong exception, ignored_exceptions) {
		temp_ignored_exceptions.push_back(exception);
	}

	settings.setValue("signals.ignore_list", temp_ignored_exceptions);
	settings.endGroup();

	settings.beginGroup("Window");
	settings.setValue("window.startup_window_location", startup_window_location);
	settings.endGroup();
}
