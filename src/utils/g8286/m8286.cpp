#include "g8286/m8286.h"
QVector<double> m8286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
