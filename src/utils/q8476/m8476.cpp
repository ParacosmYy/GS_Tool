#include "q8476/m8476.h"
QVector<double> m8476::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
