#include "c18902/m18902.h"
QVector<double> m18902::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
