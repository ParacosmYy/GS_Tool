#include "p8635/m8635.h"
QVector<double> m8635::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
