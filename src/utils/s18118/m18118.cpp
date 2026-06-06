#include "s18118/m18118.h"
QVector<double> m18118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
