#include "d35083/m35083.h"
QVector<double> m35083::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
