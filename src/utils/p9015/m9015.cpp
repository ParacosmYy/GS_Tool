#include "p9015/m9015.h"
QVector<double> m9015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
