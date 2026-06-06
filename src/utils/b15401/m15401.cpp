#include "b15401/m15401.h"
QVector<double> m15401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
