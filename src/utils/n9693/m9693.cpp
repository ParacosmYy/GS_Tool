#include "n9693/m9693.h"
QVector<double> m9693::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
