#include "f35005/m35005.h"
QVector<double> m35005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
