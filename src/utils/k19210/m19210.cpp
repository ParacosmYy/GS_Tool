#include "k19210/m19210.h"
QVector<double> m19210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
