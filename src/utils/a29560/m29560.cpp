#include "a29560/m29560.h"
QVector<double> m29560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
