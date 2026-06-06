#include "b35741/m35741.h"
QVector<double> m35741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
