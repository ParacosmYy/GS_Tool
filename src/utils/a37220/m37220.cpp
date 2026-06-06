#include "a37220/m37220.h"
QVector<double> m37220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
