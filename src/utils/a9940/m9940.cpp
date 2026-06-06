#include "a9940/m9940.h"
QVector<double> m9940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
