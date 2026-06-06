#include "k21310/m21310.h"
QVector<double> m21310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
