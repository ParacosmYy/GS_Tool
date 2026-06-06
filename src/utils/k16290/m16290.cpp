#include "k16290/m16290.h"
QVector<double> m16290::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
