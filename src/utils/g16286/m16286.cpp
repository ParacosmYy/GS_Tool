#include "g16286/m16286.h"
QVector<double> m16286::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
