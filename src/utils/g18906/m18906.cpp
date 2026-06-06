#include "g18906/m18906.h"
QVector<double> m18906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
