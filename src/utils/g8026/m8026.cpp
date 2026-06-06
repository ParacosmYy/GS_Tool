#include "g8026/m8026.h"
QVector<double> m8026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
