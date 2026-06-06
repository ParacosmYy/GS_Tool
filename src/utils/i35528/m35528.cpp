#include "i35528/m35528.h"
QVector<double> m35528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
