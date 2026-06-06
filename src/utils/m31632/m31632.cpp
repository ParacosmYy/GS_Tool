#include "m31632/m31632.h"
QVector<double> m31632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
