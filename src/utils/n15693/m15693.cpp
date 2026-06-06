#include "n15693/m15693.h"
QVector<double> m15693::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
