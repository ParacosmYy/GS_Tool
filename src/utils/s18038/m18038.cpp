#include "s18038/m18038.h"
QVector<double> m18038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
