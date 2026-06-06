#include "m17632/m17632.h"
QVector<double> m17632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
