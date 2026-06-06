#include "f25105/m25105.h"
QVector<double> m25105::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
