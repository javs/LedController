#include <array>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>

//! Simple fixed size FIR filter
template<std::size_t Size>
struct FIRFilter {
    using SampleProducer = std::function<uint16_t()>;

    FIRFilter(SampleProducer sample) : m_SampleProducer{std::move(sample)}
    {
        // A basic linear curve higher on newer samples
        int total = 0;
        for (std::size_t i = 0; i < Size; ++i)
            total += i +1;

        for (std::size_t i = 0; i < Size; ++i)
            m_Coefficients[i] = static_cast<float>(Size - i) / total;
    }
    
    //! Process a new input sample and return the filtered output
    uint16_t ProcessSample(uint16_t input)
    {
        m_Samples[m_Index] = input;

        // Apply FIR
        double output = 0.0;
        for (size_t i = 0; i < Size; ++i) {
            output += m_Coefficients[i] * m_Samples[(m_Index + Size - i) % Size];
        }

        // Update the index for circular buffering
        m_Index = (m_Index + 1) % Size;

        return static_cast<uint16_t>(std::clamp(output, 0.0, 65535.0));
    }


    //! Set the filter coefficients
    void SetCoefficients(const std::array<double, Size>& newCoefficients)
    {
        m_Coefficients = newCoefficients;
    }

    //! Get the next sample
    uint16_t Sample()
    {
        return ProcessSample(m_SampleProducer());
    }

private:
    std::array<double, Size>    m_Coefficients;     //!< Filter coefficients
    std::array<uint16_t, Size>  m_Samples {0};      //!< Input samples circular buffer
    size_t                      m_Index {0};        //!< Current index for circular buffer
    SampleProducer              m_SampleProducer;   //!< Function that produces the next sample
};
