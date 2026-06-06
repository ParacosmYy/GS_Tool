#include "s9678/m9678.h"
QVector<double> m9678::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
