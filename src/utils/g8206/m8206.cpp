#include "g8206/m8206.h"
QVector<double> m8206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
