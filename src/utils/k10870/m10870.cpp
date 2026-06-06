#include "k10870/m10870.h"
QVector<double> m10870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
