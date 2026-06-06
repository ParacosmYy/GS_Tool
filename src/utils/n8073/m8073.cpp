#include "n8073/m8073.h"
QVector<double> m8073::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
