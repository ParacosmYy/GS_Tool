#include "p8315/m8315.h"
QVector<double> m8315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
