#include "g8146/m8146.h"
QVector<double> m8146::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
