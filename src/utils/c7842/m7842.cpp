#include "c7842/m7842.h"
QVector<double> m7842::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
