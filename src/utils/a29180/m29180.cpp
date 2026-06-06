#include "a29180/m29180.h"
QVector<double> m29180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
