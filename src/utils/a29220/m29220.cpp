#include "a29220/m29220.h"
QVector<double> m29220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
