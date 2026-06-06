#include "k29210/m29210.h"
QVector<double> m29210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
