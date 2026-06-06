#include "c8742/m8742.h"
QVector<double> m8742::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
