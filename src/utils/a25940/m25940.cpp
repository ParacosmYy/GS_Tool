#include "a25940/m25940.h"
QVector<double> m25940::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
