#include "c7962/m7962.h"
QVector<double> m7962::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
