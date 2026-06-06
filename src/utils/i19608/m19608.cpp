#include "i19608/m19608.h"
QVector<double> m19608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
