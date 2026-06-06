#include "a19180/m19180.h"
QVector<double> m19180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
