#include "g18526/m18526.h"
QVector<double> m18526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
