#include "i36608/m36608.h"
QVector<double> m36608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
