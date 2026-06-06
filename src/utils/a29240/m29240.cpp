#include "a29240/m29240.h"
QVector<double> m29240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
