#include "b17601/m17601.h"
QVector<double> m17601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
