#include "h24127/m24127.h"
QVector<double> m24127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
