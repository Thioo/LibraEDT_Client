#ifndef LINEARMOVER_H
#define LINEARMOVER_H
#include <vector>
#include <Windows.h>


class CLinearMover {
public:
    struct PointData {
        float total;
        float distance;
        cv::Point2f p1;
        cv::Point2f p2;

        PointData()
            : total(0.f),
            distance(0.f) {

        }

        PointData(float total, float distance, cv::Point2f p1, cv::Point2f p2)
            : total(total),
            distance(distance),
            p1(p1),
            p2(p2) {

        }
    };
private:
	//std::vector<POINT> m_points;
	std::vector<cv::Point2f> m_points;

    std::vector<PointData> m_distances;
   //int m_duration;
    float m_duration;
    //int m_elapsed;
    float m_elapsed;
    float m_length;

public:

    CLinearMover()
        :
        m_duration(0),
        m_elapsed(0) {}

    CLinearMover(std::initializer_list<cv::Point2f> points, float duration);
	//void UpdateMoverData(POINT being, POINT end, int duration) { m_points.clear();  m_points.push_back(being); m_points.push_back(end); this->m_duration = duration; this->m_elapsed = 0; updateLength(); }
   // void UpdateMoverData(cv::Point2f being, cv::Point2f end, float duration) { m_points.clear();  m_points.push_back(being); m_points.push_back(end); this->m_duration = duration; this->m_elapsed = 0; updateLength(); }
    std::vector<cv::Point2f>& UpdateMoverData(cv::Point2f being, cv::Point2f end, float duration, bool bReset = true)
    { 
        if(bReset)
			this->m_elapsed = 0;

        m_points.clear();
		m_points.push_back(being);
        m_points.push_back(end); 
        this->m_duration = duration;
        updateLength();
        return m_points;
    }
    
    template<typename iter_t>  
    CLinearMover(iter_t begin, iter_t end, float duration )
        : m_points(begin, end),
        m_duration(duration),
        m_elapsed(0) {
        updateLength();
    }

    CLinearMover(const CLinearMover&) = default;

    CLinearMover& operator=(const CLinearMover&) = default;

	//POINT update(int delta);
    cv::Point2f update(float delta, float fCorrectionX=0.0f, float fCorrectionY=0.0f);
    
    bool isComplete() const;

    void setDuration(int duration);

    int getDuration() const;

    int getElapsed() const;
private:
    void updateLength();
};


#endif // MOVER_H