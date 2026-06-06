#include "a29340/m29340.h"
QVector<double> m29340::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
