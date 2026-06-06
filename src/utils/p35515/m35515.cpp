#include "p35515/m35515.h"
QVector<double> m35515::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
