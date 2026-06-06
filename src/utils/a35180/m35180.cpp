#include "a35180/m35180.h"
QVector<double> m35180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
