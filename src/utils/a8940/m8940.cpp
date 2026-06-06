#include "a8940/m8940.h"
QVector<double> m8940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
