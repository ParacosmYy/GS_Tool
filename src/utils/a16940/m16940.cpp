#include "a16940/m16940.h"
QVector<double> m16940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
