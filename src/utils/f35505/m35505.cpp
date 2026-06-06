#include "f35505/m35505.h"
QVector<double> m35505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
