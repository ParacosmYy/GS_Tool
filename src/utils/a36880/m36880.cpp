#include "a36880/m36880.h"
QVector<double> m36880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
