#include "k21650/m21650.h"
QVector<double> m21650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
