#include "k9250/m9250.h"
QVector<double> m9250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
