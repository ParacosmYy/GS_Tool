#include "m9632/m9632.h"
QVector<double> m9632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
